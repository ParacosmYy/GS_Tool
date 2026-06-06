#include "d7963/m7963.h"
QVector<double> m7963::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
