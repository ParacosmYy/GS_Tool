#include "a28680/m28680.h"
QVector<double> m28680::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
