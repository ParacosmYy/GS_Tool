#include "a28220/m28220.h"
QVector<double> m28220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
