#include "k35410/m35410.h"
QVector<double> m35410::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
