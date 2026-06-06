#include "l35131/m35131.h"
QVector<double> m35131::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
