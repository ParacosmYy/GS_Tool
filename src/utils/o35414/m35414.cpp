#include "o35414/m35414.h"
QVector<double> m35414::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
