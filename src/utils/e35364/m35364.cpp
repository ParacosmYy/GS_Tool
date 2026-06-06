#include "e35364/m35364.h"
QVector<double> m35364::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
