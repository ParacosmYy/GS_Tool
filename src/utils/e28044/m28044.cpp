#include "e28044/m28044.h"
QVector<double> m28044::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
