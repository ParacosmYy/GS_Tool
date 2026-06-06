#include "m12112/m12112.h"
QVector<double> m12112::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
