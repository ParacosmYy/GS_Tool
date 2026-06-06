#include "b16721/m16721.h"
QVector<double> m16721::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
