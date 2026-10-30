#ifndef IMV_OCR_H
#define IMV_OCR_H

#include <stdbool.h>
#include <stddef.h>

#include "bitmap.h"

struct imv_ocr;

/* A single word detected by OCR, with its bounding box in image pixels */
struct imv_ocr_word {
  char *text;
  int x, y, w, h;
  float confidence;
};

/* A single character (symbol) detected by OCR, with its bounding box in
 * image pixels. Chars are stored in reading order; word_start/line_start
 * mark the first char of each word/text line. */
struct imv_ocr_char {
  char *text; /* UTF-8 bytes of the symbol */
  int x, y, w, h;
  bool word_start;
  bool line_start;
};

/* The result of running OCR on an image */
struct imv_ocr_result {
  struct imv_ocr_word *words;
  size_t count;
  struct imv_ocr_char *chars;
  size_t char_count;
  char *text; /* full extracted text */
};

/* Create an OCR instance for the given language (e.g. "eng", "por",
 * "por+eng"). Returns NULL on failure (missing tessdata, etc). */
struct imv_ocr *imv_ocr_create(const char *lang);

/* Clean up an OCR instance */
void imv_ocr_free(struct imv_ocr *ocr);

/* Run OCR on a bitmap (RGBA pixels). Returns a heap-allocated result,
 * or NULL on failure. Free with imv_ocr_result_free. */
struct imv_ocr_result *imv_ocr_run(struct imv_ocr *ocr, const struct imv_bitmap *bmp);

/* Clean up an OCR result */
void imv_ocr_result_free(struct imv_ocr_result *result);

#endif
