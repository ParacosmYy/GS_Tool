#include "i35028/m35028.h"
QVector<double> m35028::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
