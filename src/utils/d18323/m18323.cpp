#include "d18323/m18323.h"
QVector<double> m18323::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
