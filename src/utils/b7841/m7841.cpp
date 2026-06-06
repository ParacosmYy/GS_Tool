#include "b7841/m7841.h"
QVector<double> m7841::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
