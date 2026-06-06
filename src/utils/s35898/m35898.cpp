#include "s35898/m35898.h"
QVector<double> m35898::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
