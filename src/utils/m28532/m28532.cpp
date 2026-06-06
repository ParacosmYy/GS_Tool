#include "m28532/m28532.h"
QVector<double> m28532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
