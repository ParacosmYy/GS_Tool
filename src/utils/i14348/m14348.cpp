#include "i14348/m14348.h"
QVector<double> m14348::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
