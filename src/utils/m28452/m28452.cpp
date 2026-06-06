#include "m28452/m28452.h"
QVector<double> m28452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
