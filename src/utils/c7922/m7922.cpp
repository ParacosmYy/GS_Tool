#include "c7922/m7922.h"
QVector<double> m7922::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
