#include "d21583/m21583.h"
QVector<double> m21583::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
