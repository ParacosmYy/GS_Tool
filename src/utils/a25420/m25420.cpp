#include "a25420/m25420.h"
QVector<double> m25420::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
