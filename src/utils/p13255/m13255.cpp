#include "p13255/m13255.h"
QVector<double> m13255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
