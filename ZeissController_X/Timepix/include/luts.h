#ifndef ASI_LUTS_Hxx
#define ASI_LUTS_Hxx

void applyLookupTable(const int startRow, const int endRow, AsiFrame<AsiDecodeWord_t> *output);
const AsiDecodeWord_t *getLookupTable();
const AsiDecodeWord_t *getInverseLookupTable();
void updateInvalidLutEntry(bool invalidLutZero);

#endif