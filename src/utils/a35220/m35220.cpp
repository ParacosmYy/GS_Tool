#include "a35220/m35220.h"
QVector<double> m35220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
