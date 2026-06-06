#include "g18806/m18806.h"
QVector<double> m18806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
