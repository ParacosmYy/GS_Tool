#include "k7850/m7850.h"
QVector<double> m7850::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
