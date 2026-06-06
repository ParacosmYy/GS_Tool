#include "l18131/m18131.h"
QVector<double> m18131::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
