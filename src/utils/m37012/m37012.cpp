#include "m37012/m37012.h"
QVector<double> m37012::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
