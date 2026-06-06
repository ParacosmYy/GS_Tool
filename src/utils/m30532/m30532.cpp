#include "m30532/m30532.h"
QVector<double> m30532::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
