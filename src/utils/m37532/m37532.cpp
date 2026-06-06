#include "m37532/m37532.h"
QVector<double> m37532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
