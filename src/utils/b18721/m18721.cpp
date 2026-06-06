#include "b18721/m18721.h"
QVector<double> m18721::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
