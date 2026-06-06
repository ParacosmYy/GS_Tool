#include "g35106/m35106.h"
QVector<double> m35106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
