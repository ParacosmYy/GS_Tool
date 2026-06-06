#include "d36583/m36583.h"
QVector<double> m36583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
