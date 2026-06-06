#include "k18550/m18550.h"
QVector<double> m18550::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
