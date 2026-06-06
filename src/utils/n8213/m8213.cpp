#include "n8213/m8213.h"
QVector<double> m8213::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
