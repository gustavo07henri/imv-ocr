#include "ocr.h"

#include <stdlib.h>
#include <string.h>

#include <tesseract/capi.h>

struct imv_ocr {
  TessBaseAPI *api;
};

struct imv_ocr *imv_ocr_create(const char *lang)
{
  struct imv_ocr *ocr = calloc(1, sizeof *ocr);
  if (!ocr) {
    return NULL;
  }

  ocr->api = TessBaseAPICreate();
  if (!ocr->api) {
    free(ocr);
    return NULL;
  }

  if (TessBaseAPIInit3(ocr->api, NULL, lang ? lang : "eng") != 0) {
    TessBaseAPIDelete(ocr->api);
    free(ocr);
    return NULL;
  }

  return ocr;
}

void imv_ocr_free(struct imv_ocr *ocr)
{
  if (!ocr) {
    return;
  }
  TessBaseAPIEnd(ocr->api);
  TessBaseAPIDelete(ocr->api);
  free(ocr);
}

static int append_word(struct imv_ocr_result *result, size_t *capacity,
    const char *text, int left, int top, int right, int bottom, float confidence)
{
  if (result->count == *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : 64;
    struct imv_ocr_word *words = realloc(result->words,
        new_capacity * sizeof *words);
    if (!words) {
      return -1;
    }
    result->words = words;
    *capacity = new_capacity;
  }

  struct imv_ocr_word *word = &result->words[result->count];
  word->text = strdup(text);
  if (!word->text) {
    return -1;
  }
  word->x = left;
  word->y = top;
  word->w = right - left;
  word->h = bottom - top;
  word->confidence = confidence;
  result->count++;
  return 0;
}

static int append_char(struct imv_ocr_result *result, size_t *capacity,
    const char *text, int left, int top, int right, int bottom,
    bool word_start, bool line_start)
{
  if (result->char_count == *capacity) {
    size_t new_capacity = *capacity ? *capacity * 2 : 256;
    struct imv_ocr_char *chars = realloc(result->chars,
        new_capacity * sizeof *chars);
    if (!chars) {
      return -1;
    }
    result->chars = chars;
    *capacity = new_capacity;
  }

  struct imv_ocr_char *ch = &result->chars[result->char_count];
  ch->text = strdup(text);
  if (!ch->text) {
    return -1;
  }
  ch->x = left;
  ch->y = top;
  ch->w = right - left;
  ch->h = bottom - top;
  ch->word_start = word_start;
  ch->line_start = line_start;
  result->char_count++;
  return 0;
}

struct imv_ocr_result *imv_ocr_run(struct imv_ocr *ocr, const struct imv_bitmap *bmp)
{
  if (!ocr || !bmp || !bmp->data || bmp->width <= 0 || bmp->height <= 0) {
    return NULL;
  }

  TessBaseAPISetImage(ocr->api, bmp->data, bmp->width, bmp->height, 4,
      bmp->width * 4);
  TessBaseAPISetSourceResolution(ocr->api, 300);

  if (TessBaseAPIRecognize(ocr->api, NULL) != 0) {
    return NULL;
  }

  struct imv_ocr_result *result = calloc(1, sizeof *result);
  if (!result) {
    return NULL;
  }

  char *full_text = TessBaseAPIGetUTF8Text(ocr->api);
  if (full_text) {
    result->text = strdup(full_text);
    TessDeleteText(full_text);
  }

  TessResultIterator *it = TessBaseAPIGetIterator(ocr->api);
  if (it) {
    size_t capacity = 0;
    do {
      char *word_text = TessResultIteratorGetUTF8Text(it, RIL_WORD);
      if (!word_text) {
        continue;
      }

      /* skip whitespace-only tokens */
      if (strspn(word_text, " \t\n\r") == strlen(word_text)) {
        TessDeleteText(word_text);
        continue;
      }

      int left, top, right, bottom;
      const TessPageIterator *pit = TessResultIteratorGetPageIteratorConst(it);
      if (TessPageIteratorBoundingBox(pit, RIL_WORD, &left, &top, &right,
            &bottom)) {
        float confidence = TessResultIteratorConfidence(it, RIL_WORD);
        append_word(result, &capacity, word_text, left, top, right, bottom,
            confidence);
      }

      TessDeleteText(word_text);
    } while (TessResultIteratorNext(it, RIL_WORD));
    TessResultIteratorDelete(it);
  }

  /* second pass, at symbol level: per-char boxes for caret-based selection */
  it = TessBaseAPIGetIterator(ocr->api);
  if (it) {
    size_t capacity = 0;
    const TessPageIterator *pit = TessResultIteratorGetPageIteratorConst(it);
    do {
      char *symbol = TessResultIteratorGetUTF8Text(it, RIL_SYMBOL);
      if (!symbol) {
        continue;
      }

      if (strspn(symbol, " \t\n\r") == strlen(symbol)) {
        TessDeleteText(symbol);
        continue;
      }

      int left, top, right, bottom;
      if (TessPageIteratorBoundingBox(pit, RIL_SYMBOL, &left, &top, &right,
            &bottom)) {
        append_char(result, &capacity, symbol, left, top, right, bottom,
            TessPageIteratorIsAtBeginningOf(pit, RIL_WORD),
            TessPageIteratorIsAtBeginningOf(pit, RIL_TEXTLINE));
      }

      TessDeleteText(symbol);
    } while (TessResultIteratorNext(it, RIL_SYMBOL));
    TessResultIteratorDelete(it);
  }

  if (!result->text) {
    result->text = strdup("");
  }

  return result;
}

void imv_ocr_result_free(struct imv_ocr_result *result)
{
  if (!result) {
    return;
  }
  for (size_t i = 0; i < result->count; ++i) {
    free(result->words[i].text);
  }
  free(result->words);
  for (size_t i = 0; i < result->char_count; ++i) {
    free(result->chars[i].text);
  }
  free(result->chars);
  free(result->text);
  free(result);
}
