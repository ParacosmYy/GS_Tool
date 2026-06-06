#include "g8726/m8726.h"
QVector<double> m8726::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
