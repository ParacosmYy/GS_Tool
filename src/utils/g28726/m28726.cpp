#include "g28726/m28726.h"
QVector<double> m28726::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
