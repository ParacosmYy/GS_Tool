#include "m35452/m35452.h"
QVector<double> m35452::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
