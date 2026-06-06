#include "i16188/m16188.h"
QVector<double> m16188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
