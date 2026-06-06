#include "d11783/m11783.h"
QVector<double> m11783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
