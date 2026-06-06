#include "n7853/m7853.h"
QVector<double> m7853::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
