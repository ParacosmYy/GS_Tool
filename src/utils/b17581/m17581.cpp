#include "b17581/m17581.h"
QVector<double> m17581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
