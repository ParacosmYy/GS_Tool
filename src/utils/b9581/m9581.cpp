#include "b9581/m9581.h"
QVector<double> m9581::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
