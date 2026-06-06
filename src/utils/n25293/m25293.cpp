#include "n25293/m25293.h"
QVector<double> m25293::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
