#include "d18503/m18503.h"
QVector<double> m18503::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
