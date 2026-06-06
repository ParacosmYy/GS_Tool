#include "c35802/m35802.h"
QVector<double> m35802::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
