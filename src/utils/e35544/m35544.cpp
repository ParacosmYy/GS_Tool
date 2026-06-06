#include "e35544/m35544.h"
QVector<double> m35544::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
