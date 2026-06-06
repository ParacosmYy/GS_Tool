#include "k28630/m28630.h"
QVector<double> m28630::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
