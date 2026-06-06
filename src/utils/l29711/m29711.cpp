#include "l29711/m29711.h"
QVector<double> m29711::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
