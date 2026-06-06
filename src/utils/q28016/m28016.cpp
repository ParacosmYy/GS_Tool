#include "q28016/m28016.h"
QVector<double> m28016::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
