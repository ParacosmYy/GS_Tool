#include "b7941/m7941.h"
QVector<double> m7941::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
