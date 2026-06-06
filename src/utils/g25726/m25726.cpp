#include "g25726/m25726.h"
QVector<double> m25726::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
