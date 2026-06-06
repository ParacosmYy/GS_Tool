#include "l35091/m35091.h"
QVector<double> m35091::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
