#include "p17255/m17255.h"
QVector<double> m17255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
