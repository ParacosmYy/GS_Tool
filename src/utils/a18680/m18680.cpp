#include "a18680/m18680.h"
QVector<double> m18680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
