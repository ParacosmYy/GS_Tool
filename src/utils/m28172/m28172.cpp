#include "m28172/m28172.h"
QVector<double> m28172::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
