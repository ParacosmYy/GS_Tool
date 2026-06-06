#include "a28980/m28980.h"
QVector<double> m28980::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
