#include "a29760/m29760.h"
QVector<double> m29760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
