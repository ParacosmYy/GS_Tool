#include "f7885/m7885.h"
QVector<double> m7885::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
