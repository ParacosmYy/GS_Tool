#include "m32512/m32512.h"
QVector<double> m32512::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
