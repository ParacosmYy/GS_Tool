#include "k35510/m35510.h"
QVector<double> m35510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
