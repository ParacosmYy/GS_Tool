#include "k21750/m21750.h"
QVector<double> m21750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
