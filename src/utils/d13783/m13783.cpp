#include "d13783/m13783.h"
QVector<double> m13783::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
